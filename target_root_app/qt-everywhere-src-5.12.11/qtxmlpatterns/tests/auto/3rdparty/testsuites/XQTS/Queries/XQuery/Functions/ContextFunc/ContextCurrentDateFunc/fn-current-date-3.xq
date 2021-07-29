(:*******************************************************:)
(:Test: fn-current-date-3                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 1, 2005                                 :)
(:Purpose: Evaluates The "current-date" function         :)
(:as argument to fn-month-from-date function.          :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:month-from-date(fn:current-date()) 
