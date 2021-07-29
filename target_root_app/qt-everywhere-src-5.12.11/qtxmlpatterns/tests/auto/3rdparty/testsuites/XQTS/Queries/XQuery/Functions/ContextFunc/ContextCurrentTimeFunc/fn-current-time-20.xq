(:*******************************************************:)
(:Test: fn-current-time-20                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-time" function (string :)
(: value)as an argument to the fn:not function.          :) 
(:*******************************************************:)

fn:not(fn:string(fn:current-time()))