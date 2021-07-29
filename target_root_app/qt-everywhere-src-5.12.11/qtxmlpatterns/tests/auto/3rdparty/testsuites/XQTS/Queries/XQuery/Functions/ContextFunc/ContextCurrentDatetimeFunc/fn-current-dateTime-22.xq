(:*******************************************************:)
(:Test: fn-current-dateTime-22                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 1, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function     :)
(:as argument to fn:hours-from-dateTime function.        :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:hours-from-dateTime(fn:current-dateTime()) 
