#include "EpubToc.h"

static const char *TAG = "PUBINDEX";
#define PADDING 14
#define ITEMS_PER_PAGE 6

// Two virtual entries prepended before the epub's own TOC items.
static const char *VIRTUAL_ENTRIES[] = {
  "Resume Reading",
  "Return To Library",
};
static const int VIRTUAL_COUNT = 2;

static int total_items(Epub *epub)
{
  return epub->get_toc_items_count() + VIRTUAL_COUNT;
}

void EpubToc::next()
{
  if (!epub) load();
  state.selected_item = (state.selected_item + 1) % total_items(epub);
}

void EpubToc::prev()
{
  if (!epub) load();
  state.selected_item = (state.selected_item - 1 + total_items(epub)) % total_items(epub);
}

bool EpubToc::load()
{
  ESP_LOGI(TAG, "load");

  if (!epub || epub->get_path() != selected_epub.path)
  {
    renderer->show_busy();
    delete epub;

    epub = new Epub(selected_epub.path);
    if (epub->load())
    {
      ESP_LOGI(TAG, "Epub index loaded");
      return false;
    }
  }
  return true;
}

// TODO - this is currently pretty much a copy of the epub list rendering
// we can fit a lot more on the screen by allowing variable cell heights
// and a lot of the optimisations that are used for the list aren't really
// required as we're not rendering thumbnails
void EpubToc::render()
{
  ESP_LOGD(TAG, "Rendering EPUB index");
  // what page are we on?
  int current_page = state.selected_item / ITEMS_PER_PAGE;
  // show five items per page
  int cell_height = renderer->get_page_height() / ITEMS_PER_PAGE;
  int start_index = current_page * ITEMS_PER_PAGE;
  int ypos = 0;
  // starting a fresh page or rendering from scratch?
  ESP_LOGI(TAG, "Current page is %d, previous page %d, redraw=%d", current_page, state.previous_rendered_page, m_needs_redraw);
  if (current_page != state.previous_rendered_page || m_needs_redraw)
  {
    m_needs_redraw = false;
    renderer->clear_screen();
    state.previous_selected_item = -1;
    // trigger a redraw of the items
    state.previous_rendered_page = -1;
  }
  int n = total_items(epub);
  for (int i = start_index; i < start_index + ITEMS_PER_PAGE && i < n; i++)
  {
    // do we need to draw a new page of items?
    if (current_page != state.previous_rendered_page)
    {
      const char *label = (i < VIRTUAL_COUNT)
                              ? VIRTUAL_ENTRIES[i]
                              : epub->get_toc_item(i - VIRTUAL_COUNT).title.c_str();

      TextBlock *title_block = new TextBlock(LEFT_ALIGN);
      title_block->add_span(label, i < VIRTUAL_COUNT /* bold for virtual entries */, false);
      title_block->layout(renderer, epub, renderer->get_page_width());
      int text_height = cell_height - PADDING;
      int title_height = title_block->line_breaks.size() * renderer->get_line_height();
      int y_offset = title_height < text_height ? (text_height - title_height) / 2 : 0;
      int height = 0;
      for (int li = 0; li < (int)title_block->line_breaks.size() && height < text_height; li++)
      {
        title_block->render(renderer, li, 10, ypos + height + y_offset);
        height += renderer->get_line_height();
      }
      delete title_block;
    }
    // clear the selection box around the previous selected item
    if (state.previous_selected_item == i)
    {
      for (int line = 0; line < 3; line++)
      {
        renderer->draw_rect(line, ypos + PADDING / 2 + line, renderer->get_page_width() - 2 * line, cell_height - PADDING - 2 * line, 255);
      }
    }
    // draw the selection box around the current selection
    if (state.selected_item == i)
    {
      for (int line = 0; line < 3; line++)
      {
        renderer->draw_rect(line, ypos + PADDING / 2 + line, renderer->get_page_width() - 2 * line, cell_height - PADDING - 2 * line, 0);
      }
    }
    ypos += cell_height;
  }
  state.previous_selected_item = state.selected_item;
  state.previous_rendered_page = current_page;
}

int EpubToc::get_toc_action()
{
  if (state.selected_item == 0) return -1; // Resume reading
  if (state.selected_item == 1) return -2; // Return to library
  return (int)epub->get_spine_index_for_toc_index(state.selected_item - VIRTUAL_COUNT);
}