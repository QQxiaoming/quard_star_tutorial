(: Name: fn-local-name-6:)
(: Description: Evaluation of the fn:local-name function argument set computed comment node.:)
(: Use fn:count to avoid empty file :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:count(fn:local-name((comment {"A Comment Node"})))
