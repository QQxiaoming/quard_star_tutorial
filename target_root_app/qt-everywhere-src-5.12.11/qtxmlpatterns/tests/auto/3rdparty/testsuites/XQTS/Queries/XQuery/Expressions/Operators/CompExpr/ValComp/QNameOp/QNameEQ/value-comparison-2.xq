(: name : value-comparison-2 :)
(: description : Evaluation of xs:string compare to xs:anyURI.:)
(: Uses "ne" operator. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:string("example.org/") ne xs:anyURI("example.org/") 