(: Name: fn-namespace-uri-24:)
(: Description: Evaluation of the fn:namespace-uri function with argument set to a computed element node with prefix that should not used the declared defaultnamespace attribute.:)
(: Use the string function.:)

declare default element namespace "http://www.example.com/examples";
declare namespace ex = "http://www.example.com/exampleswithPrefix";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:namespace-uri(element ex:anElement {"An Element Content"}))