(:*******************************************************:)
(:Test: fn-current-date-4                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 1, 2005                                 :)
(:Purpose: Evaluates The "current-date" function used as :)
(:as argument to "day-from-date" function.               :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:day-from-date(fn:current-date())
