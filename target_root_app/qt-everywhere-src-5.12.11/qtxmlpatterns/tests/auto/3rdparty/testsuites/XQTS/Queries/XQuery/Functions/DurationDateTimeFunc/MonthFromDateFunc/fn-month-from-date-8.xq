(:*******************************************************:)
(:Test: month-from-date-8                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "month-from-date" function      :)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:month-from-date(xs:date("1970-01-01Z")) + fn:month-from-date(xs:date("1970-01-01Z"))