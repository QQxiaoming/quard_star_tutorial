(: Name: fn-root-16:)
(: Description: Evaluation of the fn:root function with argument set to computed text node by setting directly on argument.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:root(text {"A text Node"})