(:*******************************************************:)
(:Test: fn-years-from-duration-11                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P20Y10M")) div fn:years-from-duration(xs:yearMonthDuration("P05Y10M"))
