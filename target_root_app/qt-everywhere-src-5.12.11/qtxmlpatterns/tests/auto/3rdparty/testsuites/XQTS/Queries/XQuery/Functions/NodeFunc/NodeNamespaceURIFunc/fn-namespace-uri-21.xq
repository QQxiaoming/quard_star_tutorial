(: Name: fn-namespace-uri-21:)
(: Description: Evaluation of the fn:namespace-uri function with argument set to a direct element node with prefix that should use a declared namespace attribute.:)
(: Use the string function.:)

declare namespace ex = "http://www.example.com/examples";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:namespace-uri(<ex:anElement>An Element Content</ex:anElement>))