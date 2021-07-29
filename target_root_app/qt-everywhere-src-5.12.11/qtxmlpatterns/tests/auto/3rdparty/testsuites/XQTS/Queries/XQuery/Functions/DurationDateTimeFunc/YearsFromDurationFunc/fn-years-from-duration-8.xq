(:*******************************************************:)
(:Test: fn-years-from-duration-8                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P21Y10M")) + fn:years-from-duration(xs:yearMonthDuration("P22Y10M"))