(: name : value-comparison-1 :)
(: description : Evaluation of xs:string compare to xs:anyURI.:)
(: Uses "eq" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:string("example.org/") eq xs:anyURI("example.org/") 