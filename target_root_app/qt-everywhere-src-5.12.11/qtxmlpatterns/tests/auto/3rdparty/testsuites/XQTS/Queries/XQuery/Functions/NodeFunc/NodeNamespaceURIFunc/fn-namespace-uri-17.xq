(: Name: fn-namespace-uri-17:)
(: Description: Evaluation of the fn:namespace-uri function with argument set to a direct element node with a namespace attribute.:)
(: Use the string function :)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

 fn:string(fn:namespace-uri(<anElement xmlns = "http://www.example.com/examples"></anElement>))