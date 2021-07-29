(: Name: fn-namespace-uri-19:)
(: Description: Evaluation of the fn:namespace-uri function with argument set to a computed element node (with no prefix) that should not use a declared namespace attribute.:)
(: Use the count function to avoid empty file.:)

declare namespace ex = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:count(fn:namespace-uri(element anElement {"An Element Content"}))