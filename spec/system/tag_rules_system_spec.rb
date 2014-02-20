describe "Fortitude tag rules enforcement", :type => :system do
  def widget_class_with_content(options = { }, &block)
    out = super(options, &block)
    out.class_eval { enforce_element_nesting_rules true }
    out
  end

  it "should not allow a <div> inside a <p>" do
    expect { render(widget_class_with_content { p { div } })}.to raise_error(Fortitude::Errors::InvalidElementNesting)
  end

  it "should allow a <b> inside a <p>" do
    expect(render(widget_class_with_content { p { b } })).to eq('<p><b/></p>')
  end

  it "should not enforce rules if there's a render :partial in between two elements"
end
