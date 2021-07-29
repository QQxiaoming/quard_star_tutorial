(:*******************************************************:)
(:Test: fn-current-dateTime-4                            :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 1, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function used as :)
(:as argument to "day-from-dateTime" function            :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:day-from-dateTime(fn:current-dateTime())
