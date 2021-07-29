(: Name: fn-namespace-uri-22:)
(: Description: Evaluation of the fn:namespace-uri function with argument set to a direct element node with no prefix that should use a declared defaultnamespace attribute.:)
(: Use the string function.:)

declare default element namespace "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:namespace-uri(<anElement>An Element Content</anElement>))