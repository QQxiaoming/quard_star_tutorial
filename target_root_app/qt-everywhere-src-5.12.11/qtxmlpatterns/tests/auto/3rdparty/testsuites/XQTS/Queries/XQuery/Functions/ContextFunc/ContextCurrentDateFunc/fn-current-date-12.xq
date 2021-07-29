(:*******************************************************:)
(:Test: fn-current-date-12                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                :)
(:Purpose: Evaluates string value The "current-date" as  :)
(:part of an equal expression (eq operator)              :) 
(:*******************************************************:)

fn:string(fn:current-date()) eq fn:string(fn:current-date())
