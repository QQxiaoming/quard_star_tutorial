(:*******************************************************:)
(:Test: sequence-type-12                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluate simple sequence type using castable :)
(: and double. uses "double?" on sequence of two doubles.:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1E3,2E3) castable as xs:double?