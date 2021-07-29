(:*******************************************************:)
(:Test: fn-years-from-duration-19                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 10, 2005                                    :)
(:Purpose: Evaluates The "years-from-duration" function  :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:years-from-duration(xs:yearMonthDuration("P20Y10M")) ge fn:years-from-duration(xs:yearMonthDuration("P20Y10M"))