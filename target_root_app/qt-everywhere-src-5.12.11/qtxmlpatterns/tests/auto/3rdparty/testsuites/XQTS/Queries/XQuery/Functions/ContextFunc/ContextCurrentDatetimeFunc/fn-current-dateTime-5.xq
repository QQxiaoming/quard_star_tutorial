(:*******************************************************:)
(:Test: fn-current-dateTime-5                            :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 1, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function as part :)
(:of a "-" operation.                                    :) 
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:current-dateTime() - fn:current-dateTime()
