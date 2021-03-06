describe "Rails class-loading support", :type => :rails do
  uses_rails_with_template :class_loading_system_spec

  it "should not load classes under app/views without the Views:: prefix" do
    expect_exception('the_class_should_not_load', 'NameError',
      /uninitialized constant ClassLoadingSystemSpec::ClassShouldNotLoad/i)
  end

  it "should allow me to define classes under Views:: outside of app/views, like in lib/views" do
    expect_match('lib_views', /hello: i am lib\/views/)
  end

  it "should allow me to define classes under Views:: outside of app/views, but in some other autoloaded place, like app/models" do
    expect_match('app_models', /hello: i am app\/models/)
  end

  it "should not create anonymous modules without the Views:: namespace for directories under app/views/" do
    expect_exception('some_namespace', 'NameError', /uninitialized constant SomeNamespace/)
    expect_exception('some_other_namespace', 'NameError', /uninitialized constant SomeNamespace/)
    expect_match('views_some_namespace', /Views::SomeNamespace/, :no_layout => true)
    expect_match('views_some_other_namespace', /Views::SomeNamespace::SomeOtherNamespace/, :no_layout => true)
  end

  it "should autoload widgets under app/views/" do
    expect_match('autoload_widget', /autoload_widget is here!/, :no_layout => true)
  end

  it "should autoload one widget from another" do
    expect_match('autoload_one_widget_from_another', /about to run the sub widget.*this is the sub widget.*ran the sub widget/)
  end

  it "should allow use of a widget defined in lib/ from a view widget" do
    expect_match('use_lib_widget_from_view_widget', /about to run the lib widget.*this is the lib widget.*ran the lib widget/)
  end

  it "should allow use of a widget defined on another autoload path from a view widget" do
    expect_match('use_models_widget_from_view_widget', /about to run the models widget.*this is the models widget.*ran the models widget/)
  end

  it "should not allow me to define widgets outside of app/views/" do
    expect_exception('widget_defined_outside_app_views', 'ActionView::MissingTemplate', /class_loading_system_spec\/widget_defined_outside_app_views/)
  end

  it "should not let me define a widget in a file starting with an underscore, and use it for a view" do
    expect_exception('underscore_view', 'ActionView::MissingTemplate', /class_loading_system_spec\/underscore_view/)
  end

  it "should not let me define a widget in a file starting with an underscore, and autoload it" do
    expect_exception('underscore_widget', 'NameError', /uninitialized constant Views::ClassLoadingSystemSpec::UnderscoreWidget/)
  end

  it "should not let me 'require' files in app/views without a views/ prefix" do
    expect_exception('require_loaded_underscore_widget_without_views', 'LoadError', /(cannot load such file|no such file to load)/)
  end

  it "should not let me 'require' files in app/views with a views/ prefix" do
    expect_exception('require_loaded_underscore_widget_with_views', 'LoadError', /(cannot load such file|no such file to load)/)
  end

  it "should let me render a widget defined outside of app/views/ if I use render :widget" do
    expect_match('render_widget_outside_app_views', /arbitrary_name_some_widget/)
  end
end
