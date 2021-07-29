(:*******************************************************:)
(:Test: sequence-type-21                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluate simple sequence type using instance :)
(: and boolean. uses "boolean+" on sequence of two booleans.:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(fn:true(),fn:false()) instance of xs:boolean+