(:*******************************************************:)
(:Test: fn-current-time-15                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-time" function as part :)
(:of an equal expression (ge operator)                   :) 
(:*******************************************************:)

fn:string(fn:current-time()) ge fn:string(fn:current-time())
