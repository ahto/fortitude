describe "Rails development-mode support", :type => :rails do
  uses_rails_with_template :development_mode_system_spec, :rails_env => :development

  it "should automatically reload widgets if they change on disk" do
    expect_match("reload_widget", /before_reload/)
    # The sleep is unfortunate, but required: without it, Rails will not necessarily pick up our change,
    # especially if we run multiple tests back-to-back. (There's a maximum frequency at which Rails can
    # detect distinct file changes on disk, which on the order of 1 Hz -- i.e., once a second; any more
    # than this and it fails. This is irrelevant for human-centered development, but important for
    # tests like this.)
    sleep 1
    splat_new_widget!
    expect_match("reload_widget", /after_reload/)
  end

  it "should let you change the controller, and that should work fine, too" do
    expect_match("reload_widget", /datum\s+one\s+datum/)
    sleep 1
    splat_new_controller!
    expect_match("reload_widget", /datum\s+two\s+datum/)
  end

  it "should, by default, format output" do
    expect_match("sample_output", %r{<section class="one">
  <p>hello, Jessica</p>
</section>}mi)
  end

  it "should, by default, output BEGIN/END debugging tags" do
    expect_match("sample_output", %r{<!-- BEGIN Views::DevelopmentModeSystemSpec::SampleOutput depth 0: :name => "Jessica" -->
.*
<!-- END Views::DevelopmentModeSystemSpec::SampleOutput depth 0 -->}mi)
  end

  private
  def splat_new_widget!
    reload_file = File.join(rails_server.rails_root, 'app/views/development_mode_system_spec/reload_widget.rb')
    File.open(reload_file, 'w') do |f|
      f.puts <<-EOS
class Views::DevelopmentModeSystemSpec::ReloadWidget < Fortitude::Widgets::Html5
  needs :datum

  def content
    p "after_reload: datum \#{datum} datum"
  end
end
EOS
    end
  end

  def splat_new_controller!
    controller_file = File.join(rails_server.rails_root, 'app/controllers/development_mode_system_spec_controller.rb')
    File.open(controller_file, 'w') do |f|
      f.puts <<-EOS
class DevelopmentModeSystemSpecController < ApplicationController
  def reload_widget
    @datum = "two"
  end

  def sample_output
    @name = "Jessica"
  end
end
EOS
    end
  end
end
