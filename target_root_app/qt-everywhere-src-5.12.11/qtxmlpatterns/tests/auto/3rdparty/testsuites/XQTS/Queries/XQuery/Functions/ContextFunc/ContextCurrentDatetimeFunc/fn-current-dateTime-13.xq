(:*******************************************************:)
(:Test: fn-current-dateTime-13                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function as part :)
(:of an equal expression (ne operator)                   :) 
(:*******************************************************:)

fn:string(fn:current-dateTime()) ne fn:string(fn:current-dateTime())
