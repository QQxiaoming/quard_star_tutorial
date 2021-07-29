(:*******************************************************:)
(:Test: sequence-type-10                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluate simple sequence type using castable :)
(: and integer. uses "integer?" on sequence of two integers.:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1,2) castable as xs:integer?