(: Name: fn-in-scope-prefixes-14 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to xs:string.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:string(fn:in-scope-prefixes(<anElement>Some content</anElement>)[1])