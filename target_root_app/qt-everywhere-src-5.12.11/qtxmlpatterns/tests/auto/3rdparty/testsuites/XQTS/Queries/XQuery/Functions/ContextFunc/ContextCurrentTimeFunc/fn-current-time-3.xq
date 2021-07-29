(:*******************************************************:)
(:Test: fn-current-time-3                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 1, 2005                                 :)
(:Purpose: Evaluates The "current-time" function         :)
(:as argument to fn-minutes-from-time-function.          :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:minutes-from-time(fn:current-time()) 
