(:*******************************************************:)
(:Test: fn-years-from-duration-13                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P10Y10M")) mod fn:years-from-duration(xs:yearMonthDuration("P03Y10M"))
