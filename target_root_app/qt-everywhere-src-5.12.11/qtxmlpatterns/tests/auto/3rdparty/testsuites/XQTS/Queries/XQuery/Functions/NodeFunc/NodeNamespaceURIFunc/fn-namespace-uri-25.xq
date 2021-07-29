(: Name: fn-namespace-uri-25:)
(: Description: Evaluation of the fn:namespace-uri function with argument set to a computed element node with prefix no that should used the declared default namespace attribute.:)
(: Use the string function.:)

declare default element namespace "http://www.example.com/examples";
declare namespace ex = "http://www.example.com/exampleswithPrefix";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:namespace-uri(element anElement {"An Element Content"}))