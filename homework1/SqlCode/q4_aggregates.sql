SELECT 	CategoryName,
				COUNT(*) AS cateCount,
				ROUND(AVG(UnitPrice))  AS avgUP,
				MIN(UnitPrice) AS minUP,
				MAX(UnitPrice) AS maxUP,
				SUM(UnitsOnOrder) AS totalUnitsOnOrder
				
FROM 	Product 
			INNER JOIN 
			Category 
			on CategoryId = Category.Id
GROUP BY CategoryId
HAVING cateCount>10;
