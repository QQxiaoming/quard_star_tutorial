(:*******************************************************:)
(:Test: sequence-type-5                                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: March 29, 2006                                   :)
(:Purpose: Evaluate simple sequence type using instance of :)
(: and strings. uses "integer*" on string sequence.       :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

("ab","cde","fgh") instance of xs:integer*
