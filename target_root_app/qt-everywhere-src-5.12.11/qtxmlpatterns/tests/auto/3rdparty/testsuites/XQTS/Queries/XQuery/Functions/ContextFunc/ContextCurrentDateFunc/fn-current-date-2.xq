(:*******************************************************:)
(:Test: fn-current-date                                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 1, 2005                                 :)
(:Purpose: Evaluates The "current-date" function         :)
(:as argument to fn:hours-from-date function.            :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:year-from-date(fn:current-date()) 
