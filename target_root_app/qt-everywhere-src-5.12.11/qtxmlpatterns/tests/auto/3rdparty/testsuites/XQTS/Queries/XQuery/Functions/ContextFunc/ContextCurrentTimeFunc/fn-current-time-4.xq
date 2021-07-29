(:*******************************************************:)
(:Test: fn-current-time-4                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 1, 2005                                 :)
(:Purpose: Evaluates The "current-time" function used as :)
(:as argument to "seconds-from-time" function.           :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:seconds-from-time(fn:current-time())
