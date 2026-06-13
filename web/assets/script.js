/* sprite-utils documentation — shared script */

(function () {
  'use strict';

  /* ── Theme toggle ─────────────────────────────────────────── */
  const THEME_KEY = 'su-docs-theme';

  function applyTheme(theme) {
    if (theme === 'dark') {
      document.documentElement.setAttribute('data-theme', 'dark');
    } else if (theme === 'light') {
      document.documentElement.setAttribute('data-theme', 'light');
    } else {
      document.documentElement.removeAttribute('data-theme');
    }
  }

  function getTheme() {
    return localStorage.getItem(THEME_KEY) || 'auto';
  }

  function toggleTheme() {
    const current = getTheme();
    const next = current === 'dark' ? 'light' : 'dark';
    localStorage.setItem(THEME_KEY, next);
    applyTheme(next);
    updateThemeButton();
  }

  function updateThemeButton() {
    const btn = document.getElementById('theme-toggle');
    if (!btn) return;
    const theme = getTheme();
    btn.textContent = theme === 'dark' ? '☀ Light' : '☾ Dark';
    btn.title = theme === 'dark' ? 'Switch to light mode' : 'Switch to dark mode';
  }

  // Apply saved theme before first paint
  applyTheme(getTheme());

  document.addEventListener('DOMContentLoaded', function () {
    updateThemeButton();

    const btn = document.getElementById('theme-toggle');
    if (btn) btn.addEventListener('click', toggleTheme);

    /* ── Highlight active sidebar link ──────────────────────── */
    const currentPath = window.location.pathname;
    const sidebarLinks = document.querySelectorAll('.sidebar nav a');
    sidebarLinks.forEach(function (link) {
      // Compare resolved href to current page
      const linkPath = new URL(link.href, window.location.href).pathname;
      if (linkPath === currentPath || linkPath + 'index.html' === currentPath) {
        link.classList.add('active');
      }
    });
  });
})();
