class Views::ClassLoadingSystemSpec::LoadedUnderscoreWidget < Fortitude::Widget
  class << self
    def data
      "loaded underscore widget!"
    end
  end

  def content
    p "hello, this is the loaded underscore widget"
  end
end