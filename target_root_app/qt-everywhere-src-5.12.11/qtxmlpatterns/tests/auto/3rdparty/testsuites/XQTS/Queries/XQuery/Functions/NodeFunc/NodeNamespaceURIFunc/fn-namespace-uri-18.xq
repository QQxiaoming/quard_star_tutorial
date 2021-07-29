(: Name: fn-namespace-uri-18:)
(: Description: Evaluation of the fn:namespace-uri function with argument set to a computed element node (with prefix) that uses a declared namespace attribute.:)
(: Use the string function :)

declare namespace ex = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:namespace-uri(element ex:anElement {"An Element Content"}))