#pragma once

#include <cmath>
#include <functional>
#include <iomanip>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

using StatValues = std::unordered_map<std::string, double>;

class StatExpression final
{
public:
    StatExpression() = default; // No expression: ordinary base stat.

    StatExpression(double value)
    {
        if (!std::isfinite(value))
            throw std::invalid_argument("Stat expression constant is not finite");

        std::ostringstream text;
        text.imbue(std::locale::classic());
        text << std::setprecision(17) << value;
        text_ = text.str();
        evaluate_ = [value](const StatValues&) { return value; };
    }

    static StatExpression Reference(std::string id)
    {
        if (id.empty())
            throw std::invalid_argument("Stat expression reference is empty");

        const std::string name = std::move(id);
        Evaluator read = [name](const StatValues& values)
            {
                const auto found = values.find(name);
                if (found == values.end())
                    throw std::logic_error(
                        "Missing expression dependency '" + name + "'");
                return found->second;
            };
        return StatExpression(std::move(read), name, { name });
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(evaluate_);
    }

    double Evaluate(const StatValues& values) const
    {
        if (!evaluate_)
            throw std::logic_error("Cannot evaluate an empty stat expression");
        return evaluate_(values);
    }

    const std::string& ToString() const noexcept { return text_; }

    const std::set<std::string>& Dependencies() const noexcept
    {
        return dependencies_;
    }

    friend StatExpression operator+(const StatExpression& left, const StatExpression& right)
    {
        return Combine(left, right, '+');
    }

    friend StatExpression operator-(const StatExpression& left, const StatExpression& right)
    {
        return Combine(left, right, '-');
    }

    friend StatExpression operator*(const StatExpression& left, const StatExpression& right)
    {
        return Combine(left, right, '*');
    }

    friend StatExpression operator/(const StatExpression& left, const StatExpression& right)
    {
        return Combine(left, right, '/');
    }

private:
    using Evaluator = std::function<double(const StatValues&)>;

    StatExpression(
        Evaluator evaluate,
        std::string text,
        std::set<std::string> dependencies)
        : evaluate_(std::move(evaluate)),
        text_(std::move(text)),
        dependencies_(std::move(dependencies)) {
    }

    static StatExpression Combine(
        const StatExpression& left,
        const StatExpression& right,
        char operation)
    {
        if (!left || !right)
            throw std::invalid_argument("Empty stat expression operand");

        std::set<std::string> dependencies = left.dependencies_;
        dependencies.insert(right.dependencies_.begin(), right.dependencies_.end());

        Evaluator evaluateLeft = left.evaluate_;
        Evaluator evaluateRight = right.evaluate_;
        Evaluator combined = [evaluateLeft = std::move(evaluateLeft),
            evaluateRight = std::move(evaluateRight), operation](const StatValues& values)
            {
                const double a = evaluateLeft(values);
                const double b = evaluateRight(values);
                double result = 0.0;

                switch (operation)
                {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/':
                    if (b == 0.0)
                        throw std::domain_error("Division by zero in stat expression");
                    result = a / b;
                    break;
                default:
                    throw std::logic_error("Unknown stat expression operator");
                }

                if (!std::isfinite(result))
                    throw std::domain_error("Non-finite stat expression result");
                return result;
            };

        std::string text = "(" + left.text_ + " " + std::string(1, operation) +
            " " + right.text_ + ")";
        return StatExpression(std::move(combined), std::move(text), std::move(dependencies));
    }

    Evaluator evaluate_;
    std::string text_;
    std::set<std::string> dependencies_;
};

inline StatExpression Stat(std::string id)
{
    return StatExpression::Reference(std::move(id));
}