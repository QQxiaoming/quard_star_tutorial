(:*******************************************************:)
(:Test: fn-years-from-duration-9                         :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P30Y10M")) - fn:years-from-duration(xs:yearMonthDuration("P10Y10M"))
