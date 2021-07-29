(:*******************************************************:)
(:Test - lessthanonanyuri-1                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: September, 25 2006                               :)
(:Purpose: Evaluation of "lt" operator on xs:anyURI datatype.:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:anyURI("http://www.example/com")) < (xs:anyURI("http://www.example/com"))
