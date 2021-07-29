(:*******************************************************:)
(:Test: fn-current-time-12                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5,  2005                                :)
(:Purpose: Evaluates string value The "current-time" as  :)
(:part of a "numeric-equal" expression (eq operator)     :) 
(:*******************************************************:)

fn:string(fn:current-time()) eq fn:string(fn:current-time())
