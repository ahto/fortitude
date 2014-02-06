require 'fortitude/rendering_context'

::ActiveSupport::SafeBuffer.class_eval do
  alias_method :fortitude_concat, :original_concat
  public :fortitude_concat
end

module Fortitude
  module Rails
    class Renderer
      class << self
        # TODO: Refactor this and render :widget => ... support into one method somewhere.
        def render(widget_class, template_handler, local_assigns, is_partial)
          widget = widget_class.new(template_handler.assigns.merge(local_assigns).with_indifferent_access)
          template_handler.with_output_buffer do
            output = ""
            rendering_context = ::Fortitude::RenderingContext.new(output, template_handler)
=begin
            widget.to_html(rendering_context)
            template_handler.output_buffer << rendering_context.output
=end
            widget.to_html(rendering_context.output)
            template_handler.output_buffer << output.html_safe
          end
        end
      end
    end
  end
end
