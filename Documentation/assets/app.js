(() => {
  'use strict';
  const wiki = window.GOBO_DESIGNER_WIKI;
  const nav = document.querySelector('#nav');
  const article = document.querySelector('#article');
  const search = document.querySelector('#search');
  const status = document.querySelector('#search-status');
  const byId = new Map(wiki.pages.map(page => [page.id, page]));
  const plain = html => {
    const node = document.createElement('div');
    node.innerHTML = html;
    return (node.textContent || '').replace(/\s+/g, ' ').trim();
  };
  const index = new Map(wiki.pages.map(page => [page.id,
    (page.title + ' ' + page.summary + ' ' + plain(page.content)).toLowerCase()]));
  function route() {
    let value;
    try { value = decodeURIComponent(location.hash.slice(1)); }
    catch (_) { value = 'home'; }
    const [id, section] = value.split('/');
    return { page: byId.get(id) || byId.get('home'), section };
  }
  function render(scroll = false) {
    const { page, section } = route();
    document.title = page.title + ' · Gobo Designer Guide';
    article.innerHTML = '<p class="eyebrow">' + page.section + '</p><h1>' +
      page.title + '</h1><p class="summary">' + page.summary + '</p>' + page.content +
      '<footer>Source review: ' + wiki.reviewed + ' · <a href="#home">Guide home</a></footer>';
    const headings = [...article.querySelectorAll('h2')];
    headings.forEach((heading, i) => { heading.id = 'section-' + (i + 1); });
    if (headings.length > 1) {
      const toc = document.createElement('aside');
      toc.className = 'contents';
      toc.setAttribute('aria-label', 'On this page');
      const label = document.createElement('strong');
      label.textContent = 'On this page';
      toc.append(label);
      const list = document.createElement('ul');
      headings.forEach(heading => {
        const item = document.createElement('li');
        const anchor = document.createElement('a');
        anchor.href = '#' + page.id + '/' + heading.id;
        anchor.textContent = heading.textContent;
        item.append(anchor);
        list.append(item);
      });
      toc.append(list);
      article.querySelector('.summary').after(toc);
    }
    nav.querySelectorAll('a').forEach(anchor => {
      const active = anchor.dataset.page === page.id;
      anchor.classList.toggle('active', active);
      if (active) anchor.setAttribute('aria-current', 'page');
      else anchor.removeAttribute('aria-current');
    });
    if (scroll) {
      const target = section && headings.find(h => h.id === section);
      if (target) target.scrollIntoView({ block: 'start' });
      else { article.focus({ preventScroll: true }); window.scrollTo(0, 0); }
    }
  }
  function draw() {
    const terms = search.value.trim().toLowerCase().split(/\s+/).filter(Boolean);
    let count = 0;
    nav.replaceChildren();
    wiki.groups.forEach(group => {
      const pages = group.pages.map(id => byId.get(id))
        .filter(page => page && terms.every(term => index.get(page.id).includes(term)));
      if (!pages.length) return;
      const heading = document.createElement('h2');
      heading.textContent = group.title;
      nav.append(heading);
      pages.forEach(page => {
        count++;
        const anchor = document.createElement('a');
        anchor.href = '#' + page.id;
        anchor.dataset.page = page.id;
        anchor.textContent = page.title;
        if (terms.length) {
          const summary = document.createElement('small');
          summary.textContent = page.summary;
          anchor.append(summary);
        }
        nav.append(anchor);
      });
    });
    status.textContent = terms.length
      ? count + ' matching page' + (count === 1 ? '' : 's') +
        '. Search includes article text and function names.'
      : wiki.pages.length + ' pages · Search topics or function names';
    if (!count) {
      const empty = document.createElement('p');
      empty.textContent = 'No matches. Try a shorter term, such as hover, collision, or Play.';
      nav.append(empty);
    }
    render();
  }
  search.addEventListener('input', draw);
  search.addEventListener('keydown', event => {
    if (event.key === 'Escape') { search.value = ''; draw(); }
  });
  window.addEventListener('hashchange', () => render(true));
  draw();
  if (route().section) render(true);
})();
