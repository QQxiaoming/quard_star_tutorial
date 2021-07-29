(:*******************************************************:)
(:Test:CastableAs650                                     :)
(:Written By: Nicolae Brinza                             :)
(:Purpose:The expression should not raise a dynamic error:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

count($input-context//employee[if (salary castable as xs:integer) 
  then xs:integer(salary) gt 65000 
  else false()])
