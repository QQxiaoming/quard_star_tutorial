(:*******************************************************:)
(:Test: fn-current-time-5                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 1, 2005                                 :)
(:Purpose: Evaluates The "current-time" function as part :)
(:of a "-" operation.                                    :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:current-time() - fn:current-time()
