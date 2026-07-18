import { Layout } from '../context/types';

export const layoutHasSidebar = (layout: Layout): boolean =>
  layout !== 'bottom-bar-only';

export const layoutHasBottomBar = (layout: Layout): boolean =>
  layout === 'right-sidebar-bottom-bar' ||
  layout === 'left-sidebar-bottom-bar' ||
  layout === 'bottom-bar-only';

// the standalone bar uses the tall "stacked" style; alongside a sidebar
// it's the narrow inline bar. Both offer four slots — the unit budget
// (utils/budget.ts) is what keeps configurations to a sane size.
export const layoutHasStackedBar = (layout: Layout): boolean =>
  layout === 'bottom-bar-only';

export const layoutSidebarOnLeft = (layout: Layout): boolean =>
  layout === 'left-sidebar' || layout === 'left-sidebar-bottom-bar';
