(:*******************************************************:)
(:Test: fn-current-dateTime-2                            :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 1, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function     :)
(:as argument to fn:year-from-dateTime function.        :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:year-from-dateTime(fn:current-dateTime()) 
