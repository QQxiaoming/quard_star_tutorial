(:*******************************************************:)
(:Test: secondss-from-dateTime-13                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-dateTime" function:)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:seconds-from-dateTime(xs:dateTime("1970-01-01T10:10:10Z")) mod fn:seconds-from-dateTime(xs:dateTime("1970-01-01T03:10:10Z"))
