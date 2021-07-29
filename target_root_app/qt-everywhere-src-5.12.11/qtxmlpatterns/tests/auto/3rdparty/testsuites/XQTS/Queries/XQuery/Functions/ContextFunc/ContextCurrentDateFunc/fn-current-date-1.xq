(:*******************************************************:)
(:Test: fn-current-date-1                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 1, 2005                                 :)
(:Purpose: Evaluates a simple call to the fn:current-date" :)
(: function.  Uses a String Value.                         :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:current-date())