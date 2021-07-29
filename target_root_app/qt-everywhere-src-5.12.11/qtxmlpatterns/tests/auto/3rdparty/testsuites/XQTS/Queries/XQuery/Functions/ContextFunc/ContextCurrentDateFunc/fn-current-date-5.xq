(:*******************************************************:)
(:Test: fn-current-date-5                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 1, 2005                                 :)
(:Purpose: Evaluates The "current-date" function as part :)
(:of a "-" operation.                                    :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:current-date() - fn:current-date()
