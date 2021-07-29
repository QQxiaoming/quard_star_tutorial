(:*******************************************************:)
(:Test:CastableAs649                                     :)
(:Written By:Nicolae Brinza                              :)
(:Purpose:Reordering the predicates should not cause the :)
(: expression to raise an error                          :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

count($input-context//employee[salary castable as xs:integer][xs:integer(salary) gt 65000])
