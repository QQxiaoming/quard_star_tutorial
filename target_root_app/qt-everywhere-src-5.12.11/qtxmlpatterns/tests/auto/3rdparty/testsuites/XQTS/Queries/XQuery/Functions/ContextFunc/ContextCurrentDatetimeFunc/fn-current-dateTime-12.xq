(:*******************************************************:)
(:Test: fn-current-dateTime-12                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                             :)
(:Purpose: Evaluates string value The "current-dateTime" as  :)
(:part of an equal expression (eq operator)              :) 
(:*******************************************************:)

fn:string(fn:current-dateTime()) eq fn:string(fn:current-dateTime())
