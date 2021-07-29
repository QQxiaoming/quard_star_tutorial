(: Name: fn-local-name-7:)
(: Description: Evaluation of the fn:local-name function argument set to a directly constructed comment node.:)
(: Use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:count(fn:local-name(<!-- A Comment Node -->))
