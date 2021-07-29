(:*******************************************************:)
(:Test: fn-current-dateTime-24                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 1, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function used as :)
(:as argument to "seconds-from-dateTime" function        :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:seconds-from-dateTime(fn:current-dateTime())
