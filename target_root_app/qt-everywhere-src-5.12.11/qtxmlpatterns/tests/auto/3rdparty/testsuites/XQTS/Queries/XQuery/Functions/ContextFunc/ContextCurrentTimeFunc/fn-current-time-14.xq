(:*******************************************************:)
(:Test: fn-current-time-14                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-time" function as part :)
(:of an equal expression (le operator)                   :) 
(:*******************************************************:)

fn:string(fn:current-time()) le fn:string(fn:current-time())