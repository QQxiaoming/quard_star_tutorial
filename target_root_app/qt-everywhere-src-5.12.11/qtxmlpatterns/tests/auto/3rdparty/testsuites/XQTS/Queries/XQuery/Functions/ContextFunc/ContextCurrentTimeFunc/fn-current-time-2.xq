(:*******************************************************:)
(:Test: fn-curren-time                                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 1, 2005                                 :)
(:Purpose: Evaluates The "current-time" function         :)
(:as argument to fn:hours-from-time function.            :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:hours-from-time(fn:current-time()) 
