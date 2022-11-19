SELECT RegionDescription, FirstName, LastName, MAX(Employee.Birthdate) AS young
FROM Employee
	INNER JOIN EmployeeTerritory ON Employee.Id = EmployeeTerritory.EmployeeId
	INNER JOIN Territory ON TerritoryId = Territory.Id
	INNER JOIN Region ON Region.Id = RegionId
GROUP BY RegionId
ORDER BY RegionId;