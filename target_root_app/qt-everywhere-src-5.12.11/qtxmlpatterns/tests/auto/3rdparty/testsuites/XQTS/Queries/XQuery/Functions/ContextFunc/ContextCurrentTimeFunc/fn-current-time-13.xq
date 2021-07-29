(:*******************************************************:)
(:Test: fn-current-time-13                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-time" function as part :)
(:of an equal expression (ne operator)                   :) 
(:*******************************************************:)

fn:string(fn:current-time()) ne fn:string(fn:current-time())
