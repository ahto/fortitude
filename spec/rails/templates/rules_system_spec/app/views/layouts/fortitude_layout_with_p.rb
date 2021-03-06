class Views::Layouts::FortitudeLayoutWithP < Fortitude::Widgets::Html5
  doctype :html5
  enforce_element_nesting_rules true

  def content
    doctype!

    html {
      head {
        title "rails_spec_application"
      }
    }
    body {
      p {
        yield
      }
    }
  end
end
