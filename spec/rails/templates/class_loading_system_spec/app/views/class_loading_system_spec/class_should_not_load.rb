module ClassLoadingSystemSpec
  class ClassShouldNotLoad
    class << self
      def is_loaded
        "yup!"
      end
    end
  end
end
