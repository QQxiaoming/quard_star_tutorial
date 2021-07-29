(:*******************************************************:)
(:Test: fn-current-dateTime-23                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 1, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function     :)
(:as argument to fn-minutes-from-dateTime function.      :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:minutes-from-dateTime(fn:current-dateTime()) 
